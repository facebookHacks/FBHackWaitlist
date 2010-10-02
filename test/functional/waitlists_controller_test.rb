require 'test_helper'

class WaitlistsControllerTest < ActionController::TestCase
  test "should get index" do
    get :index
    assert_response :success
    assert_not_nil assigns(:waitlists)
  end

  test "should get new" do
    get :new
    assert_response :success
  end

  test "should create waitlist" do
    assert_difference('Waitlist.count') do
      post :create, :waitlist => { }
    end

    assert_redirected_to waitlist_path(assigns(:waitlist))
  end

  test "should show waitlist" do
    get :show, :id => waitlists(:one).to_param
    assert_response :success
  end

  test "should get edit" do
    get :edit, :id => waitlists(:one).to_param
    assert_response :success
  end

  test "should update waitlist" do
    put :update, :id => waitlists(:one).to_param, :waitlist => { }
    assert_redirected_to waitlist_path(assigns(:waitlist))
  end

  test "should destroy waitlist" do
    assert_difference('Waitlist.count', -1) do
      delete :destroy, :id => waitlists(:one).to_param
    end

    assert_redirected_to waitlists_path
  end
end
